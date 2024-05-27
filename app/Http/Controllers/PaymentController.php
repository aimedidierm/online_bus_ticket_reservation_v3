<?php

namespace App\Http\Controllers;

use App\Enums\PaymentStatus;
use App\Enums\TransactionStatus;
use App\Models\Payment;
use App\Models\User;
use App\Utils\ObjectFromArray;
use Illuminate\Http\Request;
use Illuminate\Support\Facades\Auth;
use Symfony\Component\HttpFoundation\Response;
use Illuminate\Http\Response as HttpResponse;
use Paypack\Paypack;

class PaymentController extends Controller
{
    /**
     * Display a listing of the resource.
     */
    public function index()
    {
        if (Auth::user()->role == 'admin') {
            $payments = Payment::get();
            $payments->load('user', 'trip');
            return view('admin.payments', ['payments' => $payments]);
        } else {
            $payments = Payment::where('user_id', Auth::id())->get();
            $payments->load('trip');
            return view('passenger.payments', ['payments' => $payments]);
        }
    }

    public function webhookVerified(Request $request)
    {
        $secret = getenv('PAYPACK_WEBHOOK_SECRET') ? getenv('PAYPACK_WEBHOOK_SECRET') : '';
        $hash = base64_encode(hash_hmac('sha256', $request->getContent(), $secret, true));
        $hmacHeader = $request->header('X-Paypack-Signature');

        if (empty($hmacHeader)) {
            return response()->json([
                'message' => 'Missing webhook signature header',
                'success' => false,
            ], Response::HTTP_UNAUTHORIZED);
        }

        if ($hash !== $hmacHeader) {
            return response()->json([
                'message' => 'Invalid webhook signature',
                'success' => false,
            ], Response::HTTP_UNAUTHORIZED);
        }

        return null;
    }

    public function processPaypackWebhook(Request $request)
    {

        $verification = $this->webhookVerified($request);

        if ($verification !== null && $verification instanceof HttpResponse) {
            return $verification;
        }

        $payment = ObjectFromArray::createObject($request->all());
        $transaction = null;

        if (isset($payment->data) && isset($payment->data->ref)) {
            $transaction = Payment::where('ref', $payment->data->ref)->first();

            if ($transaction) {
                $transaction->transaction_status = (string) $payment->data->status;


                if ($payment->data->status == TransactionStatus::SUCCESSFUL->value) {
                    $admin = User::where('role', 'admin')->first();

                    $transaction->status = PaymentStatus::PAYED->value;

                    $this->paypackConfig()->Cashout([
                        "amount" => $this->calculateAmount($payment->data->amount),
                        "phone" => $admin->phone,
                    ]);
                }
                $transaction->update();
            } else {
                logger()->error("Transaction with ref " . $payment->data->ref . " not found");
                return response()->json([
                    'success' => false,
                    'message' => 'Transaction with ref ' . $payment->data->ref . ' not found',
                    "ref" => $payment->data->ref ?? null,
                    "data" => $payment->data ?? null,
                ], Response::HTTP_NOT_FOUND);
            }
        }

        return response()->json([
            'message' => 'Webhook processed successfully',
            'success' => true,
        ], Response::HTTP_OK);
    }

    private function calculateAmount(int $amount)
    {
        $fees = $amount * 5 / 100;
        $amountAfterFees = $amount - $fees;
        return $amountAfterFees;
    }

    public function paypackConfig()
    {
        $paypack = new Paypack();

        $paypack->config([
            'client_id' => env('PAYPACK_CLIENT_ID'),
            'client_secret' => env('PAYPACK_CLIENT_SECRET'),
        ]);

        return $paypack;
    }
}

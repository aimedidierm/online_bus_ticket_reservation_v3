<?php

use App\Http\Controllers\HardwareController;
use App\Http\Controllers\PaymentController;
use Illuminate\Http\Request;
use Illuminate\Support\Facades\Route;

/*
|--------------------------------------------------------------------------
| API Routes
|--------------------------------------------------------------------------
|
| Here is where you can register API routes for your application. These
| routes are loaded by the RouteServiceProvider and all of them will
| be assigned to the "api" middleware group. Make something great!
|
*/

Route::middleware('auth:sanctum')->get('/user', function (Request $request) {
    return $request->user();
});

Route::post('/status', [HardwareController::class, 'show']);

Route::post("/webhooks/paypack", [PaymentController::class, 'processPaypackWebhook']);
Route::get("/webhooks/paypack", function () {
    return response()->json([
        'message' => 'Paypack Webhook Ping',
        'status' => 'success',
        'datetime' => now()->toDateTimeString()
    ]);
});

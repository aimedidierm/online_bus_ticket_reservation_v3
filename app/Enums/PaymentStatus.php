<?php

namespace App\Enums;

enum PaymentStatus: string
{
    case PENDING = 'Pending';
    case PAYED = 'Payed';
    case USED = 'Used';
}

<?php

namespace App\Enums;

enum TransactionStatus: string
{
    case SUCCESSFUL = 'successful';
    case PENDING = 'pending';
    case FAILED = 'failed';
}
